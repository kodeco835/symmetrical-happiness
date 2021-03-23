import {
  ActionsListRepoWorkflowsResponseData,
  ActionsListWorkflowRunsForRepoResponseData,
  ActionsListJobsForWorkflowRunResponseData,
  PullsGetResponseData,
  IssuesCreateCommentResponseData,
} from '@octokit/types';
import { request } from '@octokit/request';
import fs from 'fs-extra';
import path from 'path';

import { execAll, filterAsync } from './Utils';
import { EXPO_DIR } from './Constants';

export type Workflow = ActionsListRepoWorkflowsResponseData['workflows'][0] & {
  slug: string;
  baseSlug: string;
  inputs?: Record<string, string>;
};
export type WorkflowRun = ActionsListWorkflowRunsForRepoResponseData['workflow_runs'][0];
export type WorkflowDispatchEventInputs = Record<string, string>;
export type Job = ActionsListJobsForWorkflowRunResponseData['jobs'][0];

/**
 * Requests for the list of active workflows.
 */
export async function getWorkflowsAsync(): Promise<Workflow[]> {
  const response = await request('GET /repos/:owner/:repo/actions/workflows', makeExpoOptions({}));

  // We need to filter out some workflows because they might have
  // - empty `name` or `path` (why?)
  // - inactive state
  // - workflow config no longer exists
  const workflows = await filterAsync(response.data.workflows, async (workflow) =>
    Boolean(
      workflow.name &&
        workflow.path &&
        workflow.state === 'active' &&
        (await fs.pathExists(path.join(EXPO_DIR, workflow.path)))
    )
  );
  return workflows
    .sort((a, b) => a.name.localeCompare(b.name))
    .map((workflow) => {
      const slug = path.basename(workflow.path, path.extname(workflow.path));
      return {
        ...workflow,
        slug,
        baseSlug: slug,
      };
    });
}

/**
 * Requests for the list of manually triggered runs for given workflow ID.
 */
export async function getWorkflowRunsAsync(
  workflowId: number,
  event?: string
): Promise<WorkflowRun[]> {
  const response = await request(
    'GET /repos/:owner/:repo/actions/runs',
    makeExpoOptions({ event })
  );
  return response.data.workflow_runs.filter(
    (workflowRun) => workflowRun.workflow_id === workflowId
  );
}

/**
 * Resolves to the recently dispatched workflow run.
 */
export async function getLatestDispatchedWorkflowRunAsync(
  workflowId: number
): Promise<WorkflowRun | null> {
  const workflowRuns = await getWorkflowRunsAsync(workflowId, 'workflow_dispatch');
  return workflowRuns[0] ?? null;
}

/**
 * Requests for the list of job for workflow run with given ID.
 */
export async function getJobsForWorkflowRunAsync(workflowRunId: number): Promise<Job[]> {
  const response = await request(
    'GET /repos/:owner/:repo/actions/runs/:run_id/jobs',
    makeExpoOptions({
      run_id: workflowRunId,
    })
  );
  return response.data.jobs;
}

/**
 * Dispatches an event that triggers a workflow with given ID or workflow filename (including extension).
 */
export async function dispatchWorkflowEventAsync(
  workflowId: number | string,
  ref: string,
  inputs?: WorkflowDispatchEventInputs
): Promise<void> {
  await request(
    'POST /repos/:owner/:repo/actions/workflows/:workflow_id/dispatches',
    // @ts-ignore It expects workflow_id to be a number, however workflow filename (string) is also supported.
    makeExpoOptions({
      workflow_id: workflowId,
      ref,
      inputs: inputs ?? {},
    })
  );
}

/**
 * Requests for the pull request object.
 */
export async function getPullRequestAsync(pullRequestId: number): Promise<PullsGetResponseData> {
  const response = await request(
    'GET /repos/:owner/:repo/pulls/:pull_number',
    makeExpoOptions({
      pull_number: pullRequestId, //10469,
    })
  );
  return response.data;
}

/**
 * Returns an array of issue IDs that has been auto-closed by the pull request.
 */
export async function getClosedIssuesAsync(pullRequestId: number): Promise<number[]> {
  const pullRequest = await getPullRequestAsync(pullRequestId);
  const matches = execAll(
    /(?:close|closes|closed|fix|fixes|fixed|resolve|resolves|resolved) (#|https:\/\/github\.com\/expo\/expo\/issues\/)(\d+)/gi,
    pullRequest.body,
    2
  );
  return matches.map((match) => parseInt(match, 10)).filter((issue) => !isNaN(issue));
}

/**
 * Creates an issue comment with given body.
 */
export async function commentOnIssueAsync(
  issueId: number,
  body: string
): Promise<IssuesCreateCommentResponseData> {
  const response = await request(
    'POST /repos/:owner/:repo/issues/:issue_number/comments',
    makeExpoOptions({
      issue_number: issueId,
      body,
    })
  );
  return response.data;
}

/**
 * Copies given object with params specific for `expo/expo` repository and with authorization token.
 */
function makeExpoOptions<T>(
  options: T & { headers?: object }
): T & { owner: string; repo: string } {
  return {
    headers: {
      authorization: `token ${process.env.GITHUB_TOKEN}`,
      ...options?.headers,
    },
    owner: 'expo',
    repo: 'expo',
    ...options,
  };
}
